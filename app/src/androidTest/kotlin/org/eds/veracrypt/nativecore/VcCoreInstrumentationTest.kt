package org.eds.veracrypt.nativecore

import android.content.Intent
import android.net.Uri
import android.os.ParcelFileDescriptor
import android.provider.DocumentsContract
import android.provider.DocumentsContract.Document
import android.provider.DocumentsContract.Root
import android.system.Os
import androidx.test.core.app.ActivityScenario
import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.platform.app.InstrumentationRegistry
import java.io.File
import java.io.FileOutputStream
import java.io.ByteArrayOutputStream
import java.nio.ByteBuffer
import java.util.UUID
import java.util.concurrent.CountDownLatch
import java.util.concurrent.TimeUnit
import java.util.zip.ZipEntry
import java.util.zip.ZipOutputStream
import java8.nio.file.StandardCopyOption
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.delay
import kotlinx.coroutines.runBlocking
import kotlinx.coroutines.withTimeout
import org.eds.veracrypt.documents.FileTransferManager
import org.eds.veracrypt.documents.TransferDirection
import org.eds.veracrypt.documents.TransferRequest
import org.eds.veracrypt.documents.TransferState
import org.eds.veracrypt.documents.UnlockedVolumeService
import org.eds.veracrypt.domain.CipherHint
import org.eds.veracrypt.domain.KdfHint
import org.eds.veracrypt.domain.SecretPassword
import org.eds.veracrypt.domain.VolumeCreateOptions
import org.eds.veracrypt.domain.VolumeCredentials
import org.eds.veracrypt.domain.VolumeAccessMode
import org.eds.veracrypt.domain.VolumeFileSystem
import org.eds.veracrypt.domain.VolumeKind
import org.eds.veracrypt.domain.VolumeOpenOptions
import org.eds.veracrypt.domain.VolumeOpenTarget
import org.eds.veracrypt.domain.VolumeSessionState
import org.eds.veracrypt.ui.ContainerCatalogActivity
import org.eds.veracrypt.ui.FileManagerIntents
import me.zhanghai.android.files.filelist.FileListActivity
import me.zhanghai.android.files.file.fileProviderUri
import me.zhanghai.android.files.file.formatLong
import me.zhanghai.android.files.filejob.openArchiveOutputChannel
import me.zhanghai.android.files.provider.common.copyTo
import me.zhanghai.android.files.provider.common.createFile
import me.zhanghai.android.files.provider.common.delete
import me.zhanghai.android.files.provider.common.moveTo
import me.zhanghai.android.files.provider.common.newOutputStream
import me.zhanghai.android.files.provider.common.observe
import me.zhanghai.android.files.provider.common.readAllBytes
import me.zhanghai.android.files.provider.document.createDocumentTreeRootPath
import me.zhanghai.android.files.provider.archive.createArchiveRootPath
import org.threeten.bp.Instant
import org.eds.veracrypt.session.ManagedVolumeSession
import org.junit.Test
import org.junit.runner.RunWith
import org.junit.Assert.assertArrayEquals
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Assert.fail
import org.junit.Assume.assumeTrue

@RunWith(AndroidJUnit4::class)
class VcCoreInstrumentationTest {
    @Test
    fun nativeInvariantSelfTestsPass() {
        assertTrue(VcCore.nativeRecommendedWorkerCount() > 0)
        val topology = VcCore.nativeCpuTopologySummary()
        assertTrue(topology.contains("allowed="))
        assertTrue(topology.contains("performance="))
        VcCoreDebug.runSelfTests()
    }

    /**
     * Exercises the >=128 KiB runtime-sized worker path for all nine Windows
     * creation suites. A byte-for-byte round trip proves that splitting
     * independent XTS data units did not alter the VeraCrypt tweak numbering
     * or cascade order.
     */
    @Test
    fun parallelXtsLargeAlignedBlocksRoundTripAcrossCreatableCiphers() {
        val context = InstrumentationRegistry.getInstrumentation().targetContext
        val instrumentation = InstrumentationRegistry.getInstrumentation()
        context.startActivity(
            Intent(context, ContainerCatalogActivity::class.java).addFlags(Intent.FLAG_ACTIVITY_NEW_TASK),
        )
        instrumentation.waitForIdleSync()
        UnlockedVolumeService.bind(context)
        UnlockedVolumeService.beginLongRunningOperation()
        try {
            CipherHint.entries.filter { it.isCreatable }.forEach { cipher ->
                val container = File(context.cacheDir, "EDS-TEST-parallel-xts-${cipher.name}.hc")
                var descriptor: ParcelFileDescriptor? = null
                var session = 0L
                val credentials = VolumeCredentials(
                    password = SecretPassword("parallel-xts-${cipher.name}".toCharArray()),
                    pim = 1,
                    kdfHint = KdfHint.PBKDF2_HMAC_SHA512,
                )
                try {
                    descriptor = ParcelFileDescriptor.open(
                        container,
                        ParcelFileDescriptor.MODE_CREATE or ParcelFileDescriptor.MODE_READ_WRITE or ParcelFileDescriptor.MODE_TRUNCATE,
                    )
                    val options = VolumeCreateOptions(
                        sizeBytes = 16L * 1024 * 1024,
                        volumeKind = VolumeKind.NORMAL,
                        cipher = cipher,
                        kdf = KdfHint.PBKDF2_HMAC_SHA512,
                        pim = 1,
                        fileSystem = VolumeFileSystem.EXFAT,
                    )
                    NativeRequestCodec.encodeCreate(options, credentials).use { request ->
                        session = request.useForJni { bytes ->
                            VcCore.nativeCreateNormal(descriptor!!.fd, bytes, intArrayOf(), NativeCreateProgress.inert())
                        }
                    }
                    val expected = ByteArray(128 * 1024) { index -> (index * 31).toByte() }
                    assertEquals(expected.size, VcCore.nativeWrite(session, 512L, expected, 0, expected.size))
                    VcCore.nativeFlush(session)
                    val actual = ByteArray(expected.size)
                    assertEquals(actual.size, VcCore.nativeRead(session, 512L, actual, 0, actual.size))
                    assertArrayEquals("Parallel XTS round-trip failed for $cipher", expected, actual)
                } finally {
                    if (session != 0L) VcCore.nativeClose(session)
                    descriptor?.close()
                    credentials.close()
                    assertTrue("Parallel-XTS test container was not removed", container.delete() || !container.exists())
                }
            }
        } finally {
            UnlockedVolumeService.endLongRunningOperation()
        }
    }

    @Test
    fun officialVeraCryptVectorsOpenNormalAndHiddenHeaders() {
        val instrumentation = InstrumentationRegistry.getInstrumentation()
        val context = instrumentation.targetContext
        context.startActivity(
            Intent(context, ContainerCatalogActivity::class.java).addFlags(Intent.FLAG_ACTIVITY_NEW_TASK),
        )
        instrumentation.waitForIdleSync()
        val vectors = File(context.filesDir, "veracrypt-vectors")
        val kdfs = linkedMapOf(
            "test.sha512.hc" to KdfHint.PBKDF2_HMAC_SHA512,
            "test.whirlpool.hc" to KdfHint.PBKDF2_HMAC_WHIRLPOOL,
            "test.sha256.hc" to KdfHint.PBKDF2_HMAC_SHA256,
            "test.blake2s.hc" to KdfHint.PBKDF2_HMAC_BLAKE2S,
            "test.streebog.hc" to KdfHint.PBKDF2_HMAC_STREEBOG,
        )
        assumeTrue("Run tools/stage-veracrypt-vectors-on-device.ps1 before this regression", kdfs.keys.all {
            File(vectors, it).isFile
        })
        UnlockedVolumeService.bind(context)
        UnlockedVolumeService.beginLongRunningOperation()
        try {
            kdfs.forEach { (name, kdf) ->
                assertOfficialVectorHeader(File(vectors, name), kdf, VolumeKind.NORMAL, "test")
                assertOfficialVectorHeader(File(vectors, name), kdf, VolumeKind.HIDDEN, "testhidden")
            }
        } finally {
            UnlockedVolumeService.endLongRunningOperation()
        }
    }

    @Test
    fun officialSha512VectorOpensNormalHeader() {
        val instrumentation = InstrumentationRegistry.getInstrumentation()
        val context = instrumentation.targetContext
        context.startActivity(
            Intent(context, ContainerCatalogActivity::class.java).addFlags(Intent.FLAG_ACTIVITY_NEW_TASK),
        )
        instrumentation.waitForIdleSync()
        val vector = File(context.filesDir, "veracrypt-vectors/test.sha512.hc")
        assumeTrue("Run tools/stage-veracrypt-vectors-on-device.ps1 before this regression", vector.isFile)
        UnlockedVolumeService.bind(context)
        UnlockedVolumeService.beginLongRunningOperation()
        try {
            assertOfficialVectorHeader(vector, KdfHint.PBKDF2_HMAC_SHA512, VolumeKind.NORMAL, "test")
        } finally {
            UnlockedVolumeService.endLongRunningOperation()
        }
    }

    @Test
    fun officialSha512VectorOpensHiddenHeader() {
        val instrumentation = InstrumentationRegistry.getInstrumentation()
        val context = instrumentation.targetContext
        context.startActivity(
            Intent(context, ContainerCatalogActivity::class.java).addFlags(Intent.FLAG_ACTIVITY_NEW_TASK),
        )
        instrumentation.waitForIdleSync()
        val vector = File(context.filesDir, "veracrypt-vectors/test.sha512.hc")
        assumeTrue("Run tools/stage-veracrypt-vectors-on-device.ps1 before this regression", vector.isFile)
        UnlockedVolumeService.bind(context)
        UnlockedVolumeService.beginLongRunningOperation()
        try {
            assertOfficialVectorHeader(vector, KdfHint.PBKDF2_HMAC_SHA512, VolumeKind.HIDDEN, "testhidden")
        } finally {
            UnlockedVolumeService.endLongRunningOperation()
        }
    }

    @Test
    fun officialSha512VectorAutoDetectsOuterAndHiddenHeaders() {
        val context = InstrumentationRegistry.getInstrumentation().targetContext
        val vector = File(context.filesDir, "veracrypt-vectors/test.sha512.hc")
        assumeTrue("Run tools/stage-veracrypt-vectors-on-device.ps1 before this regression", vector.isFile)
        assertOfficialVectorAutoHeader(vector, "test", hidden = false)
        assertOfficialVectorAutoHeader(vector, "testhidden", hidden = true)
    }

    @Test
    fun officialWhirlpoolVectorOpensNormalHeader() {
        val instrumentation = InstrumentationRegistry.getInstrumentation()
        val context = instrumentation.targetContext
        context.startActivity(
            Intent(context, ContainerCatalogActivity::class.java).addFlags(Intent.FLAG_ACTIVITY_NEW_TASK),
        )
        instrumentation.waitForIdleSync()
        val vector = File(context.filesDir, "veracrypt-vectors/test.whirlpool.hc")
        assumeTrue("Run tools/stage-veracrypt-vectors-on-device.ps1 before this regression", vector.isFile)
        UnlockedVolumeService.bind(context)
        UnlockedVolumeService.beginLongRunningOperation()
        try {
            assertOfficialVectorHeader(vector, KdfHint.PBKDF2_HMAC_WHIRLPOOL, VolumeKind.NORMAL, "test")
        } finally {
            UnlockedVolumeService.endLongRunningOperation()
        }
    }

    @Test
    fun officialSha256VectorOpensNormalHeader() {
        val instrumentation = InstrumentationRegistry.getInstrumentation()
        val context = instrumentation.targetContext
        context.startActivity(
            Intent(context, ContainerCatalogActivity::class.java).addFlags(Intent.FLAG_ACTIVITY_NEW_TASK),
        )
        instrumentation.waitForIdleSync()
        val vector = File(context.filesDir, "veracrypt-vectors/test.sha256.hc")
        assumeTrue("Run tools/stage-veracrypt-vectors-on-device.ps1 before this regression", vector.isFile)
        UnlockedVolumeService.bind(context)
        UnlockedVolumeService.beginLongRunningOperation()
        try {
            assertOfficialVectorHeader(vector, KdfHint.PBKDF2_HMAC_SHA256, VolumeKind.NORMAL, "test")
        } finally {
            UnlockedVolumeService.endLongRunningOperation()
        }
    }

    @Test
    fun officialBlake2sVectorOpensNormalHeader() {
        runOfficialVectorHeader(
            fileName = "test.blake2s.hc",
            kdf = KdfHint.PBKDF2_HMAC_BLAKE2S,
            volumeKind = VolumeKind.NORMAL,
            password = "test",
        )
    }

    @Test
    fun officialBlake2sVectorOpensHiddenHeader() {
        runOfficialVectorHeader(
            fileName = "test.blake2s.hc",
            kdf = KdfHint.PBKDF2_HMAC_BLAKE2S,
            volumeKind = VolumeKind.HIDDEN,
            password = "testhidden",
        )
    }

    @Test
    fun officialStreebogVectorOpensNormalHeader() {
        runOfficialVectorHeader(
            fileName = "test.streebog.hc",
            kdf = KdfHint.PBKDF2_HMAC_STREEBOG,
            volumeKind = VolumeKind.NORMAL,
            password = "test",
        )
    }

    @Test
    fun officialStreebogVectorOpensHiddenHeader() {
        runOfficialVectorHeader(
            fileName = "test.streebog.hc",
            kdf = KdfHint.PBKDF2_HMAC_STREEBOG,
            volumeKind = VolumeKind.HIDDEN,
            password = "testhidden",
        )
    }

    @Test
    fun officialWhirlpoolVectorOpensHiddenHeader() {
        runOfficialVectorHeader(
            fileName = "test.whirlpool.hc",
            kdf = KdfHint.PBKDF2_HMAC_WHIRLPOOL,
            volumeKind = VolumeKind.HIDDEN,
            password = "testhidden",
        )
    }

    @Test
    fun officialSha256VectorOpensHiddenHeader() {
        runOfficialVectorHeader(
            fileName = "test.sha256.hc",
            kdf = KdfHint.PBKDF2_HMAC_SHA256,
            volumeKind = VolumeKind.HIDDEN,
            password = "testhidden",
        )
    }

    @Test
    fun officialSha512VectorRejectsWrongPim() {
        val context = InstrumentationRegistry.getInstrumentation().targetContext
        val vector = File(context.filesDir, "veracrypt-vectors/test.sha512.hc")
        assumeTrue("Run tools/stage-veracrypt-vectors-on-device.ps1 before this regression", vector.isFile)
        val credentials = VolumeCredentials(
            password = SecretPassword("test".toCharArray()),
            pim = 1,
            kdfHint = KdfHint.PBKDF2_HMAC_SHA512,
        )
        var descriptor: ParcelFileDescriptor? = null
        try {
            descriptor = ParcelFileDescriptor.open(vector, ParcelFileDescriptor.MODE_READ_ONLY)
            val options = VolumeOpenOptions(accessMode = VolumeAccessMode.READ_ONLY)
            NativeRequestCodec.encodeOpen(options, credentials).use { request ->
                request.useForJni { bytes ->
                    assertCoreFailure(VcCoreFailure.INVALID_CREDENTIALS_OR_FORMAT) {
                        VcCore.nativeOpen(descriptor!!.fd, false, bytes, intArrayOf())
                    }
                }
            }
        } finally {
            descriptor?.close()
            credentials.close()
        }
    }

    @Test
    fun argon2idCreatedVolumeReopensWithTheSelectedKdf() {
        val context = InstrumentationRegistry.getInstrumentation().targetContext
        context.startActivity(Intent(context, ContainerCatalogActivity::class.java).addFlags(Intent.FLAG_ACTIVITY_NEW_TASK))
        InstrumentationRegistry.getInstrumentation().waitForIdleSync()
        UnlockedVolumeService.bind(context)
        UnlockedVolumeService.beginLongRunningOperation()
        val container = File(context.cacheDir, "EDS-TEST-argon2id-reopen.hc")
        val payload = ByteArray(4096) { index -> (index * 37 + 11).toByte() }
        var descriptor: ParcelFileDescriptor? = null
        var session = 0L
        try {
            descriptor = ParcelFileDescriptor.open(
                container,
                ParcelFileDescriptor.MODE_CREATE or ParcelFileDescriptor.MODE_READ_WRITE or ParcelFileDescriptor.MODE_TRUNCATE,
            )
            VolumeCredentials(
                password = SecretPassword("argon2id-test".toCharArray()),
                pim = 1,
                kdfHint = KdfHint.ARGON2ID,
            ).use { credentials ->
                val options = VolumeCreateOptions(
                    sizeBytes = 16L * 1024 * 1024,
                    volumeKind = VolumeKind.NORMAL,
                    cipher = CipherHint.AES,
                    kdf = KdfHint.ARGON2ID,
                    pim = 1,
                    fileSystem = VolumeFileSystem.EXFAT,
                )
                NativeRequestCodec.encodeCreate(options, credentials).use { request ->
                    session = request.useForJni { bytes ->
                        VcCore.nativeCreateNormal(descriptor!!.fd, bytes, intArrayOf(), NativeCreateProgress.inert())
                    }
                }
            }
            assertEquals(2, VcCore.nativeMountFileSystem(session))
            assertEquals(payload.size, VcCore.nativeWrite(session, 0, payload, 0, payload.size))
            VcCore.nativeFlush(session)
            VcCore.nativeClose(session)
            session = 0L
            descriptor?.close()
            descriptor = null

            descriptor = ParcelFileDescriptor.open(container, ParcelFileDescriptor.MODE_READ_WRITE)
            VolumeCredentials(
                password = SecretPassword("argon2id-test".toCharArray()),
                pim = 1,
                kdfHint = KdfHint.ARGON2ID,
            ).use { credentials ->
                NativeRequestCodec.encodeOpen(
                    VolumeOpenOptions(cipherHint = CipherHint.AES, accessMode = VolumeAccessMode.READ_WRITE),
                    credentials,
                ).use { request ->
                    session = request.useForJni { bytes ->
                        VcCore.nativeOpen(descriptor!!.fd, true, bytes, intArrayOf())
                    }
                }
            }
            assertEquals(2, VcCore.nativeMountFileSystem(session))
            val actual = ByteArray(payload.size)
            assertEquals(actual.size, VcCore.nativeRead(session, 0, actual, 0, actual.size))
            assertArrayEquals(payload, actual)
        } finally {
            if (session != 0L) VcCore.nativeClose(session)
            descriptor?.close()
            UnlockedVolumeService.endLongRunningOperation()
            assertTrue(container.delete() || !container.exists())
        }
    }

    @Test
    fun normalVolumeCreationCanBeCancelledAtNativeProgressCallback() {
        val context = InstrumentationRegistry.getInstrumentation().targetContext
        val container = File(context.cacheDir, "EDS-TEST-create-cancel.hc")
        var descriptor: ParcelFileDescriptor? = null
        val credentials = VolumeCredentials(password = SecretPassword("cancel-test".toCharArray()), pim = 1)
        try {
            descriptor = ParcelFileDescriptor.open(
                container,
                ParcelFileDescriptor.MODE_CREATE or ParcelFileDescriptor.MODE_READ_WRITE or ParcelFileDescriptor.MODE_TRUNCATE,
            )
            val options = VolumeCreateOptions(
                sizeBytes = 16L * 1024 * 1024,
                volumeKind = VolumeKind.NORMAL,
                cipher = CipherHint.AES,
                kdf = KdfHint.PBKDF2_HMAC_SHA512,
                pim = 1,
                fileSystem = VolumeFileSystem.EXFAT,
            )
            NativeRequestCodec.encodeCreate(options, credentials).use { request ->
                try {
                    request.useForJni { bytes ->
                        VcCore.nativeCreateNormal(
                            descriptor!!.fd,
                            bytes,
                            intArrayOf(),
                            NativeCreateProgress(onUpdate = { _, _, _ -> false }),
                        )
                    }
                    fail("A cancelled native creation must not return a session")
                } catch (failure: VcCoreFailure) {
                    assertEquals(VcCoreFailure.CANCELLED, failure.code)
                }
            }
        } finally {
            descriptor?.close()
            credentials.close()
            container.delete()
        }
    }

    @Test
    fun documentsProviderSupportsWritableExfatMutations() {
        val instrumentation = InstrumentationRegistry.getInstrumentation()
        val context = instrumentation.targetContext
        context.startActivity(
            Intent(context, ContainerCatalogActivity::class.java).addFlags(Intent.FLAG_ACTIVITY_NEW_TASK),
        )
        instrumentation.waitForIdleSync()
        val resolver = context.contentResolver
        val authority = "${context.packageName}.unlocked"
        val container = File(context.cacheDir, "EDS-TEST-documents-provider.hc")
        var descriptor: ParcelFileDescriptor? = null
        var nativeSession = 0L
        var session: ManagedVolumeSession? = null
        val credentials = VolumeCredentials(password = SecretPassword("provider-test".toCharArray()), pim = 1)
        UnlockedVolumeService.bind(context)
        UnlockedVolumeService.beginLongRunningOperation()
        try {
            descriptor = ParcelFileDescriptor.open(
                container,
                ParcelFileDescriptor.MODE_CREATE or ParcelFileDescriptor.MODE_READ_WRITE or ParcelFileDescriptor.MODE_TRUNCATE,
            )
            val options = VolumeCreateOptions(
                sizeBytes = 16L * 1024 * 1024,
                volumeKind = VolumeKind.NORMAL,
                cipher = CipherHint.AES,
                kdf = KdfHint.PBKDF2_HMAC_SHA512,
                pim = 1,
                fileSystem = VolumeFileSystem.EXFAT,
            )
            NativeRequestCodec.encodeCreate(options, credentials).use { request ->
                nativeSession = request.useForJni { bytes ->
                    VcCore.nativeCreateNormal(descriptor!!.fd, bytes, intArrayOf(), NativeCreateProgress.inert())
                }
            }
            val sessionHandle = nativeSession
            session = ManagedVolumeSession(
                sessionHandle,
                VolumeAccessMode.READ_WRITE,
                VolumeKind.NORMAL,
                CoroutineScope(SupervisorJob() + Dispatchers.IO),
            ) { VcCore.nativeClose(sessionHandle) }
            assertEquals(VolumeFileSystem.EXFAT, checkNotNull(session).mountFileSystem())
            nativeSession = 0L
            val volume = UnlockedVolumeService.volumes.add(UUID.randomUUID(), "EDS test volume", session)

            // Exercise the production hand-off rather than only the provider API: Material
            // Files must receive a live, opaque DocumentsProvider tree and never a native path.
            val treeUri = checkNotNull(UnlockedVolumeService.rootTreeUri(session))
            assertTrue(UnlockedVolumeService.isTreeUriOwnedByUnlockedVolume(context, treeUri))
            ActivityScenario.launch<FileListActivity>(
                FileManagerIntents.unlockedVolume(context, treeUri),
            ).use { scenario ->
                instrumentation.waitForIdleSync()
                scenario.onActivity { activity -> assertFalse(activity.isFinishing) }
            }

            val rootDocumentId = resolver.query(
                DocumentsContract.buildRootsUri(authority),
                arrayOf(Root.COLUMN_ROOT_ID, Root.COLUMN_DOCUMENT_ID),
                null,
                null,
                null,
            )!!.use { roots ->
                assertTrue(roots.moveToFirst())
                roots.getString(roots.getColumnIndexOrThrow(Root.COLUMN_DOCUMENT_ID))
            }
            val rootUri = DocumentsContract.buildDocumentUri(authority, rootDocumentId)
            val directoryUri = checkNotNull(DocumentsContract.createDocument(resolver, rootUri, Document.MIME_TYPE_DIR, "data"))
            var fileUri = checkNotNull(DocumentsContract.createDocument(resolver, directoryUri, "application/octet-stream", "entry.bin"))
            val payload = byteArrayOf(1, 2, 3, 4, 5, 6)
            ParcelFileDescriptor.AutoCloseOutputStream(resolver.openFileDescriptor(fileUri, "w")!!).use { output ->
                output.write(payload)
                output.flush()
                output.fd.sync()
            }
            assertEquals(payload.size.toLong(), session.stat("data/entry.bin").sizeBytes)
            resolver.openFileDescriptor(fileUri, "r")!!.use { file ->
                val randomSlice = ByteArray(3)
                assertEquals(randomSlice.size, Os.pread(file.fileDescriptor, randomSlice, 0, randomSlice.size, 2))
                assertArrayEquals(byteArrayOf(3, 4, 5), randomSlice)
            }
            ParcelFileDescriptor.AutoCloseInputStream(resolver.openFileDescriptor(fileUri, "r")!!).use { input ->
                val actual = input.readBytes()
                assertArrayEquals(payload, actual)
            }
            // Material Files must not call the provider's unimplemented copyDocument().
            // Its streaming fallback is also the route used for container/local/SAF copies.
            val documentRoot = treeUri.createDocumentTreeRootPath()
            val dataPath = documentRoot.resolve("data")
            fun assertDataRefresh(message: String, operation: () -> Unit) {
                val refreshLatch = CountDownLatch(1)
                dataPath.observe(0L).use { observable ->
                    observable.addObserver { refreshLatch.countDown() }
                    operation()
                    assertTrue(message, refreshLatch.await(5L, TimeUnit.SECONDS))
                }
            }
            assertDataRefresh("Container directory was not refreshed after file creation") {
                val createdPath = dataPath.resolve("created-by-file-manager.txt")
                createdPath.createFile()
                assertEquals(
                    0L,
                    checkNotNull(session).stat("data/created-by-file-manager.txt").sizeBytes,
                )
                createdPath.delete()
            }
            val sourcePath = documentRoot.resolve("data/entry.bin")
            val copiedPath = documentRoot.resolve("data/copied.bin")
            assertDataRefresh("Container directory was not refreshed after copying") {
                sourcePath.copyTo(copiedPath)
            }
            assertArrayEquals(payload, copiedPath.readAllBytes())
            // Sharing an unlocked file must be readable through the internal provider URI,
            // without materializing plaintext in public storage.
            resolver.openInputStream(sourcePath.fileProviderUri)!!.use { input ->
                assertArrayEquals(payload, input.readBytes())
            }
            val movedPath = documentRoot.resolve("data/moved.bin")
            assertDataRefresh("Container directory was not refreshed after moving") {
                copiedPath.moveTo(movedPath, StandardCopyOption.ATOMIC_MOVE)
            }
            assertArrayEquals(payload, movedPath.readAllBytes())
            movedPath.delete()

            // Exercise the text editor's stream read/write path through the
            // same tree URI used by the UI.
            val textUri = checkNotNull(
                DocumentsContract.createDocument(resolver, directoryUri, "text/plain", "note.txt")
            )
            val firstText = "容匣文本编辑".toByteArray()
            ParcelFileDescriptor.AutoCloseOutputStream(resolver.openFileDescriptor(textUri, "wt")!!).use {
                it.write(firstText)
                it.flush()
                it.fd.sync()
            }
            val textPath = documentRoot.resolve("data/note.txt")
            assertArrayEquals(firstText, textPath.readAllBytes())
            val savedText = "已保存的容匣文本".toByteArray()
            textPath.newOutputStream().use { it.write(savedText) }
            assertArrayEquals(savedText, textPath.readAllBytes())
            val lastModified = resolver.query(
                textUri,
                arrayOf(Document.COLUMN_LAST_MODIFIED),
                null,
                null,
                null,
            )!!.use { cursor ->
                assertTrue(cursor.moveToFirst())
                cursor.getLong(cursor.getColumnIndexOrThrow(Document.COLUMN_LAST_MODIFIED))
            }
            assertTrue("Created file has an epoch timestamp: $lastModified", lastModified >= 946_684_800_000L)
            // The Properties page formats this value on the main thread. It
            // must not depend on unregistered ThreeTenABP timezone assets.
            assertTrue(Instant.ofEpochMilli(lastModified).formatLong().isNotBlank())

            // Create an archive through the exact output-channel branch used
            // by the file manager, then reopen it through the archive browser.
            val createdArchivePayload = "created archive payload".toByteArray()
            val createdArchiveBytes = ByteArrayOutputStream().use { bytes ->
                ZipOutputStream(bytes).use { zip ->
                    zip.putNextEntry(ZipEntry("note.txt"))
                    zip.write(createdArchivePayload)
                    zip.closeEntry()
                }
                bytes.toByteArray()
            }
            val createdArchivePath = documentRoot.resolve("data/created.zip")
            assertDataRefresh("Container directory was not refreshed after archive creation") {
                createdArchivePath.openArchiveOutputChannel().use { channel ->
                    val buffer = ByteBuffer.wrap(createdArchiveBytes)
                    while (buffer.hasRemaining()) {
                        channel.write(buffer)
                    }
                }
            }
            assertArrayEquals(
                createdArchivePayload,
                createdArchivePath.createArchiveRootPath().resolve("note.txt").readAllBytes(),
            )

            // ZIP extraction uses the same seekable proxy descriptor as 7z.
            val zippedPayload = "archive payload".toByteArray()
            val zipBytes = ByteArrayOutputStream().use { bytes ->
                ZipOutputStream(bytes).use { zip ->
                    zip.putNextEntry(ZipEntry("inside.txt"))
                    zip.write(zippedPayload)
                    zip.closeEntry()
                }
                bytes.toByteArray()
            }
            val archiveUri = checkNotNull(
                DocumentsContract.createDocument(resolver, directoryUri, "application/zip", "sample.zip")
            )
            ParcelFileDescriptor.AutoCloseOutputStream(resolver.openFileDescriptor(archiveUri, "wt")!!).use {
                it.write(zipBytes)
            }
            val archiveRoot = documentRoot.resolve("data/sample.zip").createArchiveRootPath()
            val extractedPath = documentRoot.resolve("data/extracted.txt")
            archiveRoot.resolve("inside.txt").copyTo(extractedPath)
            assertArrayEquals(zippedPayload, extractedPath.readAllBytes())
            extractedPath.delete()
            createdArchivePath.delete()
            textPath.delete()
            DocumentsContract.deleteDocument(resolver, archiveUri)
            fileUri = checkNotNull(DocumentsContract.renameDocument(resolver, fileUri, "renamed.bin"))
            ParcelFileDescriptor.AutoCloseOutputStream(resolver.openFileDescriptor(fileUri, "wt")!!).use { output ->
                output.write(byteArrayOf(9, 8, 7))
                output.fd.sync()
            }
            ParcelFileDescriptor.AutoCloseInputStream(resolver.openFileDescriptor(fileUri, "r")!!).use { input ->
                val actual = input.readBytes()
                assertArrayEquals(byteArrayOf(9, 8, 7), actual)
            }
            DocumentsContract.deleteDocument(resolver, fileUri)
            DocumentsContract.deleteDocument(resolver, directoryUri)
            assertTrue(session.list("").isEmpty())
            UnlockedVolumeService.volumes.close(volume.id)
            session = null
        } finally {
            session?.close()
            if (nativeSession != 0L) VcCore.nativeClose(nativeSession)
            descriptor?.close()
            credentials.close()
            UnlockedVolumeService.endLongRunningOperation()
            assertTrue("DocumentsProvider test container was not removed", container.delete() || !container.exists())
        }
    }

    /** Exercises the application-owned encrypted import path and real byte progress. */
    @Test
    fun fileTransferEncryptImportReportsCompletionAndPreservesBytes() {
        val instrumentation = InstrumentationRegistry.getInstrumentation()
        val context = instrumentation.targetContext
        context.startActivity(
            Intent(context, ContainerCatalogActivity::class.java).addFlags(Intent.FLAG_ACTIVITY_NEW_TASK),
        )
        instrumentation.waitForIdleSync()
        UnlockedVolumeService.bind(context)
        UnlockedVolumeService.beginLongRunningOperation()
        val container = File(context.cacheDir, "EDS-TEST-transfer-import.hc")
        val source = File(context.cacheDir, "EDS-TEST-transfer-source.bin")
        val source2 = File(context.cacheDir, "EDS-TEST-transfer-source-2.bin")
        val providerSource = File(context.filesDir, "transfer-target/EDS-TEST-transfer-source.bin")
        var descriptor: ParcelFileDescriptor? = null
        var nativeSession = 0L
        var session: ManagedVolumeSession? = null
        val credentials = VolumeCredentials(password = SecretPassword("transfer-test".toCharArray()), pim = 1)
        val payload = ByteArray(512 * 1024) { index -> (index * 17 + 3).toByte() }
        val payload2 = ByteArray(128 * 1024) { index -> (index * 29 + 7).toByte() }
        val duplicateName = "${source.name.substringBeforeLast('.') } (1).${source.name.substringAfterLast('.')}"
        try {
            FileOutputStream(source).use { it.write(payload) }
            FileOutputStream(source2).use { it.write(payload2) }
            providerSource.parentFile?.mkdirs()
            providerSource.writeBytes(payload)
            descriptor = ParcelFileDescriptor.open(
                container,
                ParcelFileDescriptor.MODE_CREATE or ParcelFileDescriptor.MODE_READ_WRITE or ParcelFileDescriptor.MODE_TRUNCATE,
            )
            val options = VolumeCreateOptions(
                sizeBytes = 16L * 1024 * 1024,
                volumeKind = VolumeKind.NORMAL,
                cipher = CipherHint.AES,
                kdf = KdfHint.PBKDF2_HMAC_SHA512,
                pim = 1,
                fileSystem = VolumeFileSystem.EXFAT,
            )
            NativeRequestCodec.encodeCreate(options, credentials).use { request ->
                nativeSession = request.useForJni { bytes ->
                    VcCore.nativeCreateNormal(descriptor!!.fd, bytes, intArrayOf(), NativeCreateProgress.inert())
                }
            }
            val handle = nativeSession
            session = ManagedVolumeSession(
                handle,
                VolumeAccessMode.READ_WRITE,
                VolumeKind.NORMAL,
                CoroutineScope(SupervisorJob() + Dispatchers.IO),
            ) { VcCore.nativeClose(handle) }
            assertEquals(VolumeFileSystem.EXFAT, session.mountFileSystem())
            nativeSession = 0L
            val volume = UnlockedVolumeService.volumes.add(UUID.randomUUID(), "EDS transfer test", session)
            val authority = "${context.packageName}.unlocked"
            val rootDocumentId = resolverRootDocumentId(context, authority)
            val targetDirectory = DocumentsContract.buildTreeDocumentUri(authority, rootDocumentId)
            val request = TransferRequest(
                direction = TransferDirection.ENCRYPT_IMPORT,
                sourceUris = listOf(
                    DocumentsContract.buildDocumentUri("app.rongvault.debug.transfer", "EDS-TEST-transfer-source.bin"),
                    Uri.fromFile(source2),
                    DocumentsContract.buildDocumentUri("app.rongvault.debug.transfer", "EDS-TEST-transfer-source.bin"),
                    Uri.parse("content://app.rongvault.debug.transfer/document/failing-source"),
                ),
                targetDirectoryUri = targetDirectory,
                volumeId = volume.id,
            )
            assertTrue(FileTransferManager.start(request))
            val terminal = runBlocking {
                withTimeout(90_000L) {
                    while (true) {
                        when (val state = FileTransferManager.state.value) {
                            is TransferState.Completed,
                            is TransferState.PartialSuccess,
                            is TransferState.Failed,
                            is TransferState.Cancelled -> return@withTimeout state
                            else -> delay(100L)
                        }
                    }
                }
            }
            assertTrue("Import did not complete: $terminal", terminal is TransferState.PartialSuccess)
            assertEquals((payload.size * 2L + payload2.size), (terminal as TransferState.PartialSuccess).progress.completedBytes)
            assertEquals(1, (terminal as TransferState.PartialSuccess).progress.failures.size)
            assertEquals(payload.size.toLong(), checkNotNull(session).stat(source.name).sizeBytes)
            assertEquals(payload2.size.toLong(), checkNotNull(session).stat(source2.name).sizeBytes)
            assertEquals(payload.size.toLong(), checkNotNull(session).stat(duplicateName).sizeBytes)
            checkNotNull(session).openFile(source.name, writable = false).use { file ->
                val actual = ByteArray(payload.size)
                assertEquals(actual.size, file.read(0L, actual))
                assertArrayEquals(payload, actual)
            }
            val importedUri = findChildDocumentUri(context, targetDirectory, source.name)
            val importedUri2 = findChildDocumentUri(context, targetDirectory, source2.name)
            val importedUri3 = findChildDocumentUri(context, targetDirectory, duplicateName)
            val exportDirectory = File(context.filesDir, "transfer-target").apply { mkdirs() }
            exportDirectory.resolve(source.name).delete()
            val exportAuthority = "app.rongvault.debug.transfer"
            val exportTarget = DocumentsContract.buildDocumentUri(exportAuthority, "root")
            val probe = runCatching {
                DocumentsContract.createDocument(context.contentResolver, exportTarget, "application/octet-stream", "probe.bin")
            }
            assertTrue("Test export provider unavailable: ${probe.exceptionOrNull()}", probe.isSuccess)
            exportDirectory.resolve("probe.bin").delete()
            val exportRequest = TransferRequest(
                direction = TransferDirection.DECRYPT_EXPORT,
                sourceUris = listOf(importedUri, importedUri2, importedUri3),
                targetDirectoryUri = exportTarget,
                volumeId = volume.id,
            )
            assertTrue(FileTransferManager.start(exportRequest))
            val exportTerminal = runBlocking {
                withTimeout(90_000L) {
                    while (true) {
                        when (val state = FileTransferManager.state.value) {
                            is TransferState.Completed,
                            is TransferState.PartialSuccess,
                            is TransferState.Failed,
                            is TransferState.Cancelled -> return@withTimeout state
                            else -> delay(100L)
                        }
                    }
                }
            }
            assertTrue("Export did not complete: $exportTerminal", exportTerminal is TransferState.Completed)
            assertEquals(100, (exportTerminal as TransferState.Completed).progress.percent)
            assertArrayEquals(payload, exportDirectory.resolve(source.name).readBytes())
            assertArrayEquals(payload2, exportDirectory.resolve(source2.name).readBytes())
            assertArrayEquals(payload, exportDirectory.resolve(duplicateName).readBytes())
            exportDirectory.resolve(source.name).delete()
            exportDirectory.resolve(source2.name).delete()
            exportDirectory.resolve(duplicateName).delete()
            UnlockedVolumeService.volumes.close(volume.id)
            session = null
        } finally {
            if (FileTransferManager.hasActiveTransfer()) {
                runBlocking { FileTransferManager.cancelAndWait() }
            }
            session?.close()
            if (nativeSession != 0L) VcCore.nativeClose(nativeSession)
            descriptor?.close()
            credentials.close()
            source.delete()
            source2.delete()
            providerSource.delete()
            UnlockedVolumeService.endLongRunningOperation()
            assertTrue("Transfer test container was not removed", container.delete() || !container.exists())
        }
    }

    @Test
    fun fileTransferCancellationRemovesPartialOutputAndPublishesCancelled() {
        val instrumentation = InstrumentationRegistry.getInstrumentation()
        val context = instrumentation.targetContext
        context.startActivity(
            Intent(context, ContainerCatalogActivity::class.java).addFlags(Intent.FLAG_ACTIVITY_NEW_TASK),
        )
        instrumentation.waitForIdleSync()
        UnlockedVolumeService.bind(context)
        UnlockedVolumeService.beginLongRunningOperation()
        val container = File(context.cacheDir, "EDS-TEST-transfer-cancel.hc")
        var descriptor: ParcelFileDescriptor? = null
        var nativeSession = 0L
        var session: ManagedVolumeSession? = null
        val credentials = VolumeCredentials(password = SecretPassword("transfer-cancel".toCharArray()), pim = 1)
        try {
            descriptor = ParcelFileDescriptor.open(
                container,
                ParcelFileDescriptor.MODE_CREATE or ParcelFileDescriptor.MODE_READ_WRITE or ParcelFileDescriptor.MODE_TRUNCATE,
            )
            val options = VolumeCreateOptions(
                sizeBytes = 16L * 1024 * 1024,
                volumeKind = VolumeKind.NORMAL,
                cipher = CipherHint.AES,
                kdf = KdfHint.PBKDF2_HMAC_SHA512,
                pim = 1,
                fileSystem = VolumeFileSystem.EXFAT,
            )
            NativeRequestCodec.encodeCreate(options, credentials).use { request ->
                nativeSession = request.useForJni { bytes ->
                    VcCore.nativeCreateNormal(descriptor!!.fd, bytes, intArrayOf(), NativeCreateProgress.inert())
                }
            }
            val handle = nativeSession
            session = ManagedVolumeSession(
                handle,
                VolumeAccessMode.READ_WRITE,
                VolumeKind.NORMAL,
                CoroutineScope(SupervisorJob() + Dispatchers.IO),
            ) { VcCore.nativeClose(handle) }
            assertEquals(VolumeFileSystem.EXFAT, checkNotNull(session).mountFileSystem())
            nativeSession = 0L
            val volume = UnlockedVolumeService.volumes.add(UUID.randomUUID(), "EDS transfer cancel", session)
            val authority = "${context.packageName}.unlocked"
            val rootDocumentId = resolverRootDocumentId(context, authority)
            val targetDirectory = DocumentsContract.buildTreeDocumentUri(authority, rootDocumentId)
            val request = TransferRequest(
                direction = TransferDirection.ENCRYPT_IMPORT,
                sourceUris = listOf(Uri.parse("content://app.rongvault.debug.transfer/document/slow-source")),
                targetDirectoryUri = targetDirectory,
                volumeId = volume.id,
            )
            assertTrue(FileTransferManager.start(request))
            runBlocking {
                withTimeout(5_000L) {
                    while (FileTransferManager.notificationProgress() == null) delay(20L)
                }
            }
            val progressBeforeBackground = checkNotNull(FileTransferManager.notificationProgress())
            assertEquals("slow-source", progressBeforeBackground.currentFileName)
            assertTrue(progressBeforeBackground.currentFileBytes > 0L)
            context.startActivity(Intent(Intent.ACTION_MAIN).addCategory(Intent.CATEGORY_HOME).addFlags(Intent.FLAG_ACTIVITY_NEW_TASK))
            runBlocking { delay(300L) }
            assertTrue(FileTransferManager.hasActiveTransfer())
            assertTrue(FileTransferManager.notificationProgress() != null)
            val automation = instrumentation.uiAutomation
            automation.executeShellCommand("input keyevent 26").close()
            runBlocking { delay(350L) }
            assertTrue("Transfer stopped while the screen was off", FileTransferManager.hasActiveTransfer())
            assertTrue(FileTransferManager.notificationProgress() != null)
            automation.executeShellCommand("input keyevent 224").close()
            FileTransferManager.cancel()
            val terminal = runBlocking {
                withTimeout(10_000L) {
                    while (true) {
                        when (val state = FileTransferManager.state.value) {
                            is TransferState.Cancelled,
                            is TransferState.Completed,
                            is TransferState.PartialSuccess,
                            is TransferState.Failed -> return@withTimeout state
                            else -> delay(20L)
                        }
                    }
                }
            }
            assertTrue("Cancellation did not reach Cancelled: $terminal", terminal is TransferState.Cancelled)
            val children = DocumentsContract.buildChildDocumentsUriUsingTree(
                targetDirectory,
                DocumentsContract.getTreeDocumentId(targetDirectory),
            )
            context.contentResolver.query(
                children,
                arrayOf(Document.COLUMN_DISPLAY_NAME),
                null,
                null,
                null,
            )!!.use { cursor ->
                val nameColumn = cursor.getColumnIndexOrThrow(Document.COLUMN_DISPLAY_NAME)
                while (cursor.moveToNext()) {
                    assertFalse(cursor.getString(nameColumn).contains("slow-source"))
                }
            }
            UnlockedVolumeService.volumes.close(volume.id)
            session = null
        } finally {
            if (FileTransferManager.hasActiveTransfer()) {
                runBlocking { FileTransferManager.cancelAndWait() }
            }
            session?.close()
            if (nativeSession != 0L) VcCore.nativeClose(nativeSession)
            descriptor?.close()
            credentials.close()
            UnlockedVolumeService.endLongRunningOperation()
            assertTrue("Transfer cancellation container was not removed", container.delete() || !container.exists())
        }
    }

    @Test
    fun exfatSessionSupportsCoreFileOperations() {
        val instrumentation = InstrumentationRegistry.getInstrumentation()
        val context = instrumentation.targetContext
        context.startActivity(
            Intent(context, ContainerCatalogActivity::class.java).addFlags(Intent.FLAG_ACTIVITY_NEW_TASK),
        )
        instrumentation.waitForIdleSync()
        UnlockedVolumeService.bind(context)
        UnlockedVolumeService.beginLongRunningOperation()
        val container = File(context.cacheDir, "EDS-TEST-native-filesystem.hc")
        var descriptor: ParcelFileDescriptor? = null
        var session = 0L
        var credentials: VolumeCredentials? = null
        try {
            descriptor = ParcelFileDescriptor.open(
                container,
                ParcelFileDescriptor.MODE_CREATE or ParcelFileDescriptor.MODE_READ_WRITE or ParcelFileDescriptor.MODE_TRUNCATE,
            )
            credentials = VolumeCredentials(
                password = SecretPassword("native-test".toCharArray()),
                pim = 1,
                kdfHint = KdfHint.PBKDF2_HMAC_SHA512,
            )
            val options = VolumeCreateOptions(
                sizeBytes = 16L * 1024 * 1024,
                volumeKind = VolumeKind.NORMAL,
                cipher = CipherHint.AES,
                kdf = KdfHint.PBKDF2_HMAC_SHA512,
                pim = 1,
                fileSystem = VolumeFileSystem.EXFAT,
            )
            NativeRequestCodec.encodeCreate(options, credentials).use { request ->
                session = request.useForJni { bytes ->
                    VcCore.nativeCreateNormal(descriptor!!.fd, bytes, intArrayOf(), NativeCreateProgress.inert())
                }
            }
            assertEquals(2, VcCore.nativeMountFileSystem(session))
            assertTrue(VcCore.nativeListDirectory(session, "").isEmpty())

            // The first sub-64 KiB logical read populates one secure cache
            // page; a second read of the same data must be served from it.
            // Counters are aggregate only and expose no plaintext.
            val beforeCounters = VcCore.nativeGetPerformanceCounters(session)
            val firstRead = ByteArray(4096)
            val coldLogicalOffset = 8L * 1024 * 1024
            assertEquals(firstRead.size, VcCore.nativeRead(session, coldLogicalOffset, firstRead, 0, firstRead.size))
            val afterFirstRead = VcCore.nativeGetPerformanceCounters(session)
            assertTrue("First small read did not miss/load the cache", afterFirstRead[9] > beforeCounters[9])
            assertTrue("First small read did not issue encrypted I/O", afterFirstRead[0] > beforeCounters[0])
            val secondRead = ByteArray(firstRead.size)
            assertEquals(secondRead.size, VcCore.nativeRead(session, coldLogicalOffset, secondRead, 0, secondRead.size))
            assertArrayEquals(firstRead, secondRead)
            val afterSecondRead = VcCore.nativeGetPerformanceCounters(session)
            assertTrue("Second small read did not hit the cache", afterSecondRead[8] > afterFirstRead[8])
            assertEquals("Cache hit should avoid a second container read", afterFirstRead[0], afterSecondRead[0])

            val replacement = ByteArray(firstRead.size) { index -> (index * 7 + 19).toByte() }
            assertEquals(replacement.size, VcCore.nativeWrite(session, coldLogicalOffset, replacement, 0, replacement.size))
            val afterInvalidate = VcCore.nativeGetPerformanceCounters(session)
            val reread = ByteArray(replacement.size)
            assertEquals(reread.size, VcCore.nativeRead(session, coldLogicalOffset, reread, 0, reread.size))
            assertArrayEquals(replacement, reread)
            assertTrue("Write did not invalidate the overlapping cache page", afterInvalidate[9] == afterSecondRead[9])
            assertTrue("Read after invalidation did not reload the page", VcCore.nativeGetPerformanceCounters(session)[9] > afterInvalidate[9])

            VcCore.nativeCreateDirectory(session, "data")
            val payload = byteArrayOf(10, 20, 30, 40, 50, 60)
            val file = VcCore.nativeOpenFile(session, "data/entry.bin", writable = true, create = true, truncate = true)
            try {
                VcCore.nativePreallocateFile(file, 64 * 1024L)
                assertEquals(payload.size, VcCore.nativeWriteFile(file, 0, payload, 0, payload.size))
                VcCore.nativeFlushFile(file)
                val readBack = ByteArray(payload.size)
                assertEquals(payload.size, VcCore.nativeReadFile(file, 0, readBack, 0, readBack.size))
                assertArrayEquals(payload, readBack)
                VcCore.nativeTruncateFile(file, 3)
                VcCore.nativeFlushFile(file)
            } finally {
                VcCore.nativeCloseFile(file)
            }
            VcCore.nativeRename(session, "data/entry.bin", "data/renamed.bin")
            assertEquals(3, VcCore.nativeStat(session, "data/renamed.bin").sizeBytes)
            assertEquals(listOf("renamed.bin"), VcCore.nativeListDirectory(session, "data").map { it.name })
            VcCore.nativeDelete(session, "data/renamed.bin")
            VcCore.nativeDelete(session, "data")
            assertFalse(VcCore.nativeListDirectory(session, "").isNotEmpty())
        } finally {
            if (session != 0L) VcCore.nativeClose(session)
            descriptor?.close()
            credentials?.close()
            UnlockedVolumeService.endLongRunningOperation()
            container.delete()
        }
    }

    @Test
    fun desktopCreatedNtfsVolumeExportsReadOnlyFile() {
        val instrumentation = InstrumentationRegistry.getInstrumentation()
        val context = instrumentation.targetContext
        context.startActivity(
            Intent(context, ContainerCatalogActivity::class.java).addFlags(Intent.FLAG_ACTIVITY_NEW_TASK),
        )
        instrumentation.waitForIdleSync()
        UnlockedVolumeService.bind(context)
        val container = File(checkNotNull(context.getExternalFilesDir("ntfs")), "EDS-TEST-ntfs-transfer.hc")
        assumeTrue("Stage the explicit EDS-TEST NTFS fixture before this test", container.isFile)
        var descriptor: ParcelFileDescriptor? = null
        var nativeHandle = 0L
        var session: ManagedVolumeSession? = null
        val credentials = VolumeCredentials(
            password = SecretPassword("EDS-TEST-ntfs-transfer".toCharArray()),
            pim = 1,
            kdfHint = KdfHint.PBKDF2_HMAC_SHA512,
        )
        try {
            descriptor = ParcelFileDescriptor.open(container, ParcelFileDescriptor.MODE_READ_ONLY)
            val options = VolumeOpenOptions(
                volumeKind = VolumeKind.NORMAL,
                accessMode = VolumeAccessMode.READ_ONLY,
                cipherHint = CipherHint.AES,
            )
            NativeRequestCodec.encodeOpen(options, credentials).use { request ->
                nativeHandle = request.useForJni { bytes ->
                    VcCore.nativeOpen(descriptor!!.fd, false, bytes, intArrayOf())
                }
            }
            val handle = nativeHandle
            session = ManagedVolumeSession(
                handle,
                VolumeAccessMode.READ_ONLY,
                VolumeKind.NORMAL,
                CoroutineScope(SupervisorJob() + Dispatchers.IO),
            ) { VcCore.nativeClose(handle) }
            assertEquals(VolumeFileSystem.NTFS, checkNotNull(session).mountFileSystem())
            nativeHandle = 0L
            assertTrue(checkNotNull(session).isReadOnly)
            val volume = UnlockedVolumeService.volumes.add(UUID.randomUUID(), "EDS NTFS transfer", session)
            val authority = "${context.packageName}.unlocked"
            val rootDocumentId = resolverRootDocumentId(context, authority)
            val source = findChildDocumentUri(
                context,
                DocumentsContract.buildTreeDocumentUri(authority, rootDocumentId),
                "ntfs-origin.bin",
            )
            val exportDirectory = File(context.filesDir, "transfer-target").apply { mkdirs() }
            exportDirectory.resolve("ntfs-origin.bin").delete()
            val exportTarget = DocumentsContract.buildDocumentUri("app.rongvault.debug.transfer", "root")
            val request = TransferRequest(
                direction = TransferDirection.DECRYPT_EXPORT,
                sourceUris = listOf(source),
                targetDirectoryUri = exportTarget,
                volumeId = volume.id,
            )
            assertTrue(FileTransferManager.start(request))
            val terminal = runBlocking {
                withTimeout(30_000L) {
                    while (true) {
                        when (val state = FileTransferManager.state.value) {
                            is TransferState.Completed,
                            is TransferState.PartialSuccess,
                            is TransferState.Failed,
                            is TransferState.Cancelled -> return@withTimeout state
                            else -> delay(50L)
                        }
                    }
                }
            }
            assertTrue("NTFS export did not complete: $terminal", terminal is TransferState.Completed)
            val sha256 = java.security.MessageDigest.getInstance("SHA-256")
                .digest(exportDirectory.resolve("ntfs-origin.bin").readBytes())
                .joinToString("") { "%02X".format(it) }
            assertEquals("2A86C40AAFFF00BC96EE8A842BD07C6FBC2219AA4C4D1C36EB6B2F49A003D7BD", sha256)
            exportDirectory.resolve("ntfs-origin.bin").delete()
            UnlockedVolumeService.volumes.close(volume.id)
            session = null
        } finally {
            if (FileTransferManager.hasActiveTransfer()) runBlocking { FileTransferManager.cancelAndWait() }
            session?.close()
            if (nativeHandle != 0L) VcCore.nativeClose(nativeHandle)
            descriptor?.close()
            credentials.close()
            container.delete()
        }
    }

    @Test
    fun protectedOuterTransferStopsWhenHiddenRangeIsReached() {
        val instrumentation = InstrumentationRegistry.getInstrumentation()
        val context = instrumentation.targetContext
        context.startActivity(
            Intent(context, ContainerCatalogActivity::class.java).addFlags(Intent.FLAG_ACTIVITY_NEW_TASK),
        )
        instrumentation.waitForIdleSync()
        UnlockedVolumeService.bind(context)
        UnlockedVolumeService.beginLongRunningOperation()
        val container = File(context.cacheDir, "EDS-TEST-transfer-hidden-protection.hc")
        var descriptor: ParcelFileDescriptor? = null
        var outerHandle = 0L
        var hiddenHandle = 0L
        var protectedHandle = 0L
        var session: ManagedVolumeSession? = null
        val outerCredentials = VolumeCredentials(
            password = SecretPassword("transfer-outer".toCharArray()),
            pim = 1,
            kdfHint = KdfHint.PBKDF2_HMAC_SHA512,
        )
        val hiddenCredentials = VolumeCredentials(
            password = SecretPassword("transfer-hidden".toCharArray()),
            pim = 2,
            kdfHint = KdfHint.PBKDF2_HMAC_SHA512,
        )
        try {
            descriptor = ParcelFileDescriptor.open(
                container,
                ParcelFileDescriptor.MODE_CREATE or ParcelFileDescriptor.MODE_READ_WRITE or ParcelFileDescriptor.MODE_TRUNCATE,
            )
            val outerOptions = VolumeCreateOptions(
                sizeBytes = 32L * 1024 * 1024,
                volumeKind = VolumeKind.NORMAL,
                cipher = CipherHint.AES,
                kdf = KdfHint.PBKDF2_HMAC_SHA512,
                pim = 1,
                fileSystem = VolumeFileSystem.FAT,
            )
            NativeRequestCodec.encodeCreate(outerOptions, outerCredentials).use { request ->
                outerHandle = request.useForJni { bytes ->
                    VcCore.nativeCreateNormal(descriptor!!.fd, bytes, intArrayOf(), NativeCreateProgress.inert())
                }
            }
            assertEquals(1, VcCore.nativeMountFileSystem(outerHandle))
            val hiddenCapacity = VcCore.nativeAnalyzeHiddenCapacity(outerHandle)
            assertTrue(hiddenCapacity[0] >= 16L * 1024 * 1024)
            val hiddenOptions = outerOptions.copy(
                sizeBytes = 16L * 1024 * 1024,
                volumeKind = VolumeKind.HIDDEN,
                pim = 2,
            )
            NativeRequestCodec.encodeCreate(hiddenOptions, hiddenCredentials).use { request ->
                hiddenHandle = request.useForJni { bytes ->
                    VcCore.nativeCreateHidden(outerHandle, bytes, intArrayOf(), NativeCreateProgress.inert())
                }
            }
            VcCore.nativeClose(hiddenHandle)
            hiddenHandle = 0L
            VcCore.nativeClose(outerHandle)
            outerHandle = 0L

            val protectedOptions = VolumeOpenOptions(
                volumeKind = VolumeKind.NORMAL,
                accessMode = VolumeAccessMode.READ_WRITE,
                cipherHint = CipherHint.AES,
                hiddenVolumeProtection = hiddenCredentials,
            )
            NativeRequestCodec.encodeOpen(protectedOptions, outerCredentials).use { request ->
                protectedHandle = request.useForJni { bytes ->
                    VcCore.nativeOpen(descriptor!!.fd, true, bytes, intArrayOf())
                }
            }
            val handle = protectedHandle
            session = ManagedVolumeSession(
                handle,
                VolumeAccessMode.READ_WRITE,
                VolumeKind.NORMAL,
                CoroutineScope(SupervisorJob() + Dispatchers.IO),
            ) { VcCore.nativeClose(handle) }
            assertEquals(VolumeFileSystem.FAT, checkNotNull(session).mountFileSystem())
            protectedHandle = 0L
            val volume = UnlockedVolumeService.volumes.add(UUID.randomUUID(), "EDS protected transfer", session)
            val authority = "${context.packageName}.unlocked"
            val rootDocumentId = resolverRootDocumentId(context, authority)
            val request = TransferRequest(
                direction = TransferDirection.ENCRYPT_IMPORT,
                sourceUris = listOf(Uri.parse("content://app.rongvault.debug.transfer/document/hidden-risk-source")),
                targetDirectoryUri = DocumentsContract.buildTreeDocumentUri(authority, rootDocumentId),
                volumeId = volume.id,
            )
            assertTrue(FileTransferManager.start(request))
            val terminal = runBlocking {
                withTimeout(45_000L) {
                    while (true) {
                        when (val state = FileTransferManager.state.value) {
                            is TransferState.Failed,
                            is TransferState.Cancelled,
                            is TransferState.Completed,
                            is TransferState.PartialSuccess -> return@withTimeout state
                            else -> delay(50L)
                        }
                    }
                }
            }
            assertTrue("Protected outer transfer unexpectedly completed: $terminal", terminal is TransferState.Failed)
            assertTrue("Hidden-volume protection was not latched", checkNotNull(session).state.value is VolumeSessionState.ProtectionTriggered)
            assertTrue(checkNotNull(session).isReadOnly)
            UnlockedVolumeService.volumes.close(volume.id)
            session = null
        } finally {
            if (FileTransferManager.hasActiveTransfer()) {
                runBlocking { FileTransferManager.cancelAndWait() }
            }
            session?.close()
            if (protectedHandle != 0L) VcCore.nativeClose(protectedHandle)
            if (hiddenHandle != 0L) VcCore.nativeClose(hiddenHandle)
            if (outerHandle != 0L) VcCore.nativeClose(outerHandle)
            descriptor?.close()
            hiddenCredentials.close()
            outerCredentials.close()
            UnlockedVolumeService.endLongRunningOperation()
            assertTrue("Protected transfer container was not removed", container.delete() || !container.exists())
        }
    }

    @Test
    fun hiddenVolumeCanBeCreatedAndReopened() {
        val instrumentation = InstrumentationRegistry.getInstrumentation()
        val context = instrumentation.targetContext
        context.startActivity(
            Intent(context, ContainerCatalogActivity::class.java).addFlags(Intent.FLAG_ACTIVITY_NEW_TASK),
        )
        instrumentation.waitForIdleSync()
        val container = File(context.cacheDir, "EDS-TEST-hidden-workflow.hc")
        var descriptor: ParcelFileDescriptor? = null
        var outerSession = 0L
        var hiddenSession = 0L
        var reopenedSession = 0L
        var protectedOuterSession = 0L
        var outerCredentials: VolumeCredentials? = null
        var hiddenCredentials: VolumeCredentials? = null
        UnlockedVolumeService.beginLongRunningOperation()
        try {
            descriptor = ParcelFileDescriptor.open(
                container,
                ParcelFileDescriptor.MODE_CREATE or ParcelFileDescriptor.MODE_READ_WRITE or ParcelFileDescriptor.MODE_TRUNCATE,
            )
            outerCredentials = VolumeCredentials(
                password = SecretPassword("outer-test".toCharArray()),
                pim = 1,
                kdfHint = KdfHint.PBKDF2_HMAC_SHA512,
            )
            val outerOptions = VolumeCreateOptions(
                sizeBytes = 32L * 1024 * 1024,
                volumeKind = VolumeKind.NORMAL,
                cipher = CipherHint.AES,
                kdf = KdfHint.PBKDF2_HMAC_SHA512,
                pim = 1,
                fileSystem = VolumeFileSystem.EXFAT,
            )
            NativeRequestCodec.encodeCreate(outerOptions, outerCredentials).use { request ->
                outerSession = request.useForJni { bytes ->
                    VcCore.nativeCreateNormal(descriptor!!.fd, bytes, intArrayOf(), NativeCreateProgress.inert())
                }
            }
            assertEquals(2, VcCore.nativeMountFileSystem(outerSession))
            val capacity = VcCore.nativeAnalyzeHiddenCapacity(outerSession)
            assertEquals(2, capacity.size)
            assertTrue(capacity[0] >= 16L * 1024 * 1024)

            hiddenCredentials = VolumeCredentials(
                password = SecretPassword("hidden-test".toCharArray()),
                pim = 2,
                kdfHint = KdfHint.PBKDF2_HMAC_SHA512,
            )
            val hiddenOptions = outerOptions.copy(
                sizeBytes = 16L * 1024 * 1024,
                volumeKind = VolumeKind.HIDDEN,
                pim = 2,
            )
            NativeRequestCodec.encodeCreate(hiddenOptions, hiddenCredentials).use { request ->
                hiddenSession = request.useForJni { bytes ->
                    VcCore.nativeCreateHidden(outerSession, bytes, intArrayOf(), NativeCreateProgress.inert())
                }
            }
            assertEquals(2, VcCore.nativeMountFileSystem(hiddenSession))
            assertTrue(VcCore.nativeListDirectory(hiddenSession, "").isEmpty())
            VcCore.nativeCreateDirectory(hiddenSession, "private")
            assertEquals(listOf("private"), VcCore.nativeListDirectory(hiddenSession, "").map { it.name })

            VcCore.nativeClose(hiddenSession)
            hiddenSession = 0L
            VcCore.nativeClose(outerSession)
            outerSession = 0L

            val hiddenOpen = VolumeOpenOptions(
                volumeKind = VolumeKind.HIDDEN,
                accessMode = VolumeAccessMode.READ_WRITE,
                cipherHint = CipherHint.AES,
            )
            NativeRequestCodec.encodeOpen(hiddenOpen, hiddenCredentials).use { request ->
                reopenedSession = request.useForJni { bytes ->
                    VcCore.nativeOpen(descriptor!!.fd, true, bytes, intArrayOf())
                }
            }
            assertEquals(2, VcCore.nativeMountFileSystem(reopenedSession))
            assertEquals(listOf("private"), VcCore.nativeListDirectory(reopenedSession, "").map { it.name })

            val hiddenInfo = VcCore.nativeGetVolumeInfo(reopenedSession)
            VcCore.nativeClose(reopenedSession)
            reopenedSession = 0L
            val protectedOuterOpen = VolumeOpenOptions(
                volumeKind = VolumeKind.NORMAL,
                accessMode = VolumeAccessMode.READ_WRITE,
                cipherHint = CipherHint.AES,
                hiddenVolumeProtection = hiddenCredentials,
            )
            NativeRequestCodec.encodeOpen(protectedOuterOpen, outerCredentials).use { request ->
                protectedOuterSession = request.useForJni { bytes ->
                    VcCore.nativeOpen(descriptor!!.fd, true, bytes, intArrayOf())
                }
            }
            // Reopening the protected outer volume must still discover the
            // original exFAT filesystem before the raw protection checks.
            assertEquals(2, VcCore.nativeMountFileSystem(protectedOuterSession))
            val outerInfo = VcCore.nativeGetVolumeInfo(protectedOuterSession)
            val hiddenLogicalOffset = hiddenInfo[1] - outerInfo[1]
            assertTrue(hiddenLogicalOffset >= 0)
            assertCoreFailure(VcCoreFailure.HIDDEN_VOLUME_RISK) {
                VcCore.nativeWrite(protectedOuterSession, hiddenLogicalOffset, ByteArray(512), 0, 512)
            }
            // Protection failure latches: later writes must also be refused.
            assertCoreFailure(VcCoreFailure.HIDDEN_VOLUME_RISK) {
                VcCore.nativeWrite(protectedOuterSession, 0, ByteArray(512), 0, 512)
            }
            val readable = ByteArray(512)
            assertEquals(readable.size, VcCore.nativeRead(protectedOuterSession, 0, readable, 0, readable.size))
        } finally {
            if (protectedOuterSession != 0L) VcCore.nativeClose(protectedOuterSession)
            if (reopenedSession != 0L) VcCore.nativeClose(reopenedSession)
            if (hiddenSession != 0L) VcCore.nativeClose(hiddenSession)
            if (outerSession != 0L) VcCore.nativeClose(outerSession)
            descriptor?.close()
            hiddenCredentials?.close()
            outerCredentials?.close()
            UnlockedVolumeService.endLongRunningOperation()
            assertTrue("Hidden-workflow test container was not removed", container.delete() || !container.exists())
        }
    }

    @Test
    fun keyfileIsRequiredToReopenVolume() {
        val instrumentation = InstrumentationRegistry.getInstrumentation()
        val context = instrumentation.targetContext
        context.startActivity(
            Intent(context, ContainerCatalogActivity::class.java).addFlags(Intent.FLAG_ACTIVITY_NEW_TASK),
        )
        instrumentation.waitForIdleSync()
        UnlockedVolumeService.bind(context)
        UnlockedVolumeService.beginLongRunningOperation()
        val container = File(context.cacheDir, "EDS-TEST-keyfile.hc")
        val keyfile = File(context.cacheDir, "EDS-TEST-keyfile.bin")
        var containerDescriptor: ParcelFileDescriptor? = null
        var keyfileDescriptor: ParcelFileDescriptor? = null
        var session = 0L
        var reopenedSession = 0L
        var credentials: VolumeCredentials? = null
        try {
            FileOutputStream(keyfile).use { output -> output.write(ByteArray(64 * 1024) { it.toByte() }) }
            containerDescriptor = ParcelFileDescriptor.open(
                container,
                ParcelFileDescriptor.MODE_CREATE or ParcelFileDescriptor.MODE_READ_WRITE or ParcelFileDescriptor.MODE_TRUNCATE,
            )
            keyfileDescriptor = ParcelFileDescriptor.open(keyfile, ParcelFileDescriptor.MODE_READ_ONLY)
            credentials = VolumeCredentials(
                password = SecretPassword("keyfile-test".toCharArray()),
                pim = 1,
                kdfHint = KdfHint.PBKDF2_HMAC_SHA512,
            )
            val options = VolumeCreateOptions(
                sizeBytes = 16L * 1024 * 1024,
                volumeKind = VolumeKind.NORMAL,
                cipher = CipherHint.AES,
                kdf = KdfHint.PBKDF2_HMAC_SHA512,
                pim = 1,
                fileSystem = VolumeFileSystem.EXFAT,
            )
            NativeRequestCodec.encodeCreate(options, credentials, keyfileCount = 1).use { request ->
                session = request.useForJni { bytes ->
                    VcCore.nativeCreateNormal(containerDescriptor!!.fd, bytes, intArrayOf(keyfileDescriptor!!.fd), NativeCreateProgress.inert())
                }
            }
            assertEquals(2, VcCore.nativeMountFileSystem(session))
            VcCore.nativeClose(session)
            session = 0L

            val open = VolumeOpenOptions(
                volumeKind = VolumeKind.NORMAL,
                accessMode = VolumeAccessMode.READ_WRITE,
                cipherHint = CipherHint.AES,
            )
            NativeRequestCodec.encodeOpen(open, credentials, keyfileCount = 1).use { request ->
                reopenedSession = request.useForJni { bytes ->
                    VcCore.nativeOpen(containerDescriptor!!.fd, true, bytes, intArrayOf(keyfileDescriptor!!.fd))
                }
            }
            assertEquals(2, VcCore.nativeMountFileSystem(reopenedSession))
            VcCore.nativeClose(reopenedSession)
            reopenedSession = 0L

            NativeRequestCodec.encodeOpen(open, credentials, keyfileCount = 0).use { request ->
                request.useForJni { bytes ->
                    assertCoreFailure(VcCoreFailure.INVALID_CREDENTIALS_OR_FORMAT) {
                        VcCore.nativeOpen(containerDescriptor!!.fd, true, bytes, intArrayOf())
                    }
                }
            }
        } finally {
            if (reopenedSession != 0L) VcCore.nativeClose(reopenedSession)
            if (session != 0L) VcCore.nativeClose(session)
            keyfileDescriptor?.close()
            containerDescriptor?.close()
            credentials?.close()
            UnlockedVolumeService.endLongRunningOperation()
            assertTrue("Keyfile test container was not removed", container.delete() || !container.exists())
            assertTrue("Keyfile test input was not removed", keyfile.delete() || !keyfile.exists())
        }
    }

    private fun runOfficialVectorHeader(
        fileName: String,
        kdf: KdfHint,
        volumeKind: VolumeKind,
        password: String,
    ) {
        val instrumentation = InstrumentationRegistry.getInstrumentation()
        val context = instrumentation.targetContext
        context.startActivity(
            Intent(context, ContainerCatalogActivity::class.java).addFlags(Intent.FLAG_ACTIVITY_NEW_TASK),
        )
        instrumentation.waitForIdleSync()
        val vector = File(context.filesDir, "veracrypt-vectors/$fileName")
        assumeTrue("Run tools/stage-veracrypt-vectors-on-device.ps1 before this regression", vector.isFile)
        UnlockedVolumeService.bind(context)
        UnlockedVolumeService.beginLongRunningOperation()
        try {
            assertOfficialVectorHeader(vector, kdf, volumeKind, password)
        } finally {
            UnlockedVolumeService.endLongRunningOperation()
        }
    }

    private fun assertOfficialVectorHeader(
        container: File,
        kdf: KdfHint,
        volumeKind: VolumeKind,
        password: String,
    ) {
        var descriptor: ParcelFileDescriptor? = null
        var session = 0L
        val credentials = VolumeCredentials(
            password = SecretPassword(password.toCharArray()),
            kdfHint = kdf,
        )
        try {
            descriptor = ParcelFileDescriptor.open(container, ParcelFileDescriptor.MODE_READ_ONLY)
            val options = VolumeOpenOptions(
                cipherHint = CipherHint.AUTO,
                volumeKind = volumeKind,
                accessMode = VolumeAccessMode.READ_ONLY,
            )
            NativeRequestCodec.encodeOpen(options, credentials).use { request ->
                session = request.useForJni { bytes ->
                    VcCore.nativeOpen(descriptor!!.fd, false, bytes, intArrayOf())
                }
            }
            assertTrue("VeraCrypt vector $container did not return a session", session > 0L)
        } finally {
            if (session != 0L) VcCore.nativeClose(session)
            descriptor?.close()
            credentials.close()
        }
    }

    private fun assertOfficialVectorAutoHeader(container: File, password: String, hidden: Boolean) {
        var descriptor: ParcelFileDescriptor? = null
        var session = 0L
        val credentials = VolumeCredentials(password = SecretPassword(password.toCharArray()))
        try {
            descriptor = ParcelFileDescriptor.open(container, ParcelFileDescriptor.MODE_READ_ONLY)
            val options = VolumeOpenOptions(cipherHint = CipherHint.AUTO, target = VolumeOpenTarget.AUTO,
                accessMode = VolumeAccessMode.READ_ONLY)
            NativeRequestCodec.encodeOpen(options, credentials).use { request ->
                session = request.useForJni { bytes -> VcCore.nativeOpen(descriptor!!.fd, false, bytes, intArrayOf()) }
            }
            assertTrue(session > 0L)
            assertEquals(hidden, VcCore.nativeGetVolumeInfo(session)[6] != 0L)
        } finally {
            if (session != 0L) VcCore.nativeClose(session)
            descriptor?.close()
            credentials.close()
        }
    }

    private fun assertCoreFailure(expectedCode: Int, action: () -> Unit) {
        try {
            action()
            fail("Expected vc_core failure code $expectedCode")
        } catch (failure: VcCoreFailure) {
            assertEquals(expectedCode, failure.code)
        }
    }

    private fun resolverRootDocumentId(context: android.content.Context, authority: String): String =
        context.contentResolver.query(
            DocumentsContract.buildRootsUri(authority),
            arrayOf(Root.COLUMN_DOCUMENT_ID),
            null,
            null,
            null,
        )!!.use { roots ->
            assertTrue(roots.moveToFirst())
            roots.getString(roots.getColumnIndexOrThrow(Root.COLUMN_DOCUMENT_ID))
        }

    private fun findChildDocumentUri(
        context: android.content.Context,
        parent: Uri,
        displayName: String,
    ): Uri {
        val children = DocumentsContract.buildChildDocumentsUriUsingTree(
            parent,
            DocumentsContract.getTreeDocumentId(parent),
        )
        context.contentResolver.query(
            children,
            arrayOf(Document.COLUMN_DOCUMENT_ID, Document.COLUMN_DISPLAY_NAME),
            null,
            null,
            null,
        )!!.use { cursor ->
            val idColumn = cursor.getColumnIndexOrThrow(Document.COLUMN_DOCUMENT_ID)
            val nameColumn = cursor.getColumnIndexOrThrow(Document.COLUMN_DISPLAY_NAME)
            while (cursor.moveToNext()) {
                if (cursor.getString(nameColumn) == displayName) {
                    return DocumentsContract.buildDocumentUri(parent.authority!!, cursor.getString(idColumn))
                }
            }
        }
        fail("Created transfer output was not visible in the unlocked provider")
        error("unreachable")
    }
}
