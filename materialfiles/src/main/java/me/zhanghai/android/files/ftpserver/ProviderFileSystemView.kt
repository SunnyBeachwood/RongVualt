/*
 * Copyright (c) 2019 Hai Zhang <dreaming.in.code.zh@gmail.com>
 * All Rights Reserved.
 */

package me.zhanghai.android.files.ftpserver

import java8.nio.file.Paths
import me.zhanghai.android.files.provider.archive.isArchivePath
import me.zhanghai.android.files.navigation.RuntimeNavigationRoots
import org.apache.ftpserver.ftplet.FileSystemView
import org.apache.ftpserver.ftplet.User
import java.net.URI

class ProviderFileSystemView(private val user: User) : FileSystemView {
    private val homeDirectory: ProviderFtpFile
    private var workingDirectory: ProviderFtpFile

    init {
        val homeDirectoryPath = Paths.get(URI.create(user.homeDirectory)).normalize()
        homeDirectory = ProviderFtpFile(
            homeDirectoryPath,
            homeDirectoryPath.relativize(homeDirectoryPath),
            user,
            ::isSafePath,
            ::touchPath,
        )
        workingDirectory = homeDirectory
    }

    override fun getHomeDirectory(): ProviderFtpFile = homeDirectory

    override fun getWorkingDirectory(): ProviderFtpFile = workingDirectory

    override fun changeWorkingDirectory(directoryString: String): Boolean {
        val directory = getFile(directoryString)
        if (!directory.isDirectory) {
            return false
        }
        workingDirectory = directory
        return true
    }

    override fun getFile(fileString: String): ProviderFtpFile {
        val isAbsolute = fileString.startsWith("/")
        val homeDirectoryPath = homeDirectory.physicalFile
        val parentPath = if (isAbsolute) homeDirectoryPath else workingDirectory.physicalFile
        val relativeFileString = if (isAbsolute) fileString.drop(1) else fileString
        val filePath = parentPath.resolve(relativeFileString).normalize()
        if (!filePath.startsWith(homeDirectoryPath)) {
            return homeDirectory
        }
        return if (isSafePath(filePath)) {
            ProviderFtpFile(
                filePath,
                homeDirectoryPath.relativize(filePath),
                user,
                ::isSafePath,
                ::touchPath,
            )
        } else {
            homeDirectory
        }
    }

    override fun isRandomAccessible(): Boolean =
        // TODO: Better way of determining if the provider is random accessible.
        !homeDirectory.physicalFile.isArchivePath

    override fun dispose() {}

    /**
     * Lexical normalization alone does not stop a symlink below the shared
     * root from resolving outside it. Check every existing component without
     * following links before handing a path to the provider.
     */
    private fun isSafePath(path: java8.nio.file.Path): Boolean {
        return FtpPathPolicy.isSafe(homeDirectory.physicalFile, path)
    }

    private fun touchPath(path: java8.nio.file.Path) {
        // RuntimeNavigationRoots is a no-op for ordinary or Root paths. For
        // an unlocked volume it forwards activity to the session's auto-lock
        // timer without persisting its opaque document identifier.
        RuntimeNavigationRoots.touch(path)
    }
}
