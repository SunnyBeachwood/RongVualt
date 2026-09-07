package me.zhanghai.android.files.provider

import java8.nio.file.spi.FileSystemProvider
import me.zhanghai.android.files.provider.linux.LinuxFileSystemProvider

/** Installs the RetroFile default provider for JVM tests that use [java8.nio.file.Paths]. */
object TestFileSystemProvider {
    @Synchronized
    fun install() {
        if (FileSystemProvider.installedProviders().isEmpty()) {
            FileSystemProvider.installDefaultProvider(LinuxFileSystemProvider)
        }
    }
}
