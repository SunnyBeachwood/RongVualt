/*
 * Copyright (c) 2019 Hai Zhang <dreaming.in.code.zh@gmail.com>
 * All Rights Reserved.
 */

package me.zhanghai.android.files.provider.root

import java8.nio.file.Path
import me.zhanghai.android.files.settings.Settings
import me.zhanghai.android.files.util.valueCompat
import java.io.IOException

interface RootablePath {
    fun isRootRequired(isAttributeAccess: Boolean): Boolean
}

private val rootStrategy: RootStrategy
    get() = if (isRunningAsRoot) RootStrategy.NEVER else Settings.ROOT_STRATEGY.valueCompat

/**
 * Whether convenience stream helpers must go through the provider boundary.
 *
 * Local Linux streams intentionally use the public [java.io.FileInputStream]
 * and [java.io.FileOutputStream] APIs on Android releases where desugared NIO
 * can reach a blocked hidden API.  That shortcut is not valid for an
 * app-process RootablePath: it would silently bypass libsu and perform the
 * operation with the app UID.  Keep the decision in the same strategy
 * dispatcher used by all other provider operations.
 */
internal fun shouldUseRoot(path: Path, isAttributeAccess: Boolean = false): Boolean {
    val rootablePath = path as? RootablePath ?: return false
    return when (rootStrategy) {
        RootStrategy.NEVER -> false
        RootStrategy.AUTOMATIC -> rootablePath.isRootRequired(isAttributeAccess)
        RootStrategy.ALWAYS -> true
    }
}

@Throws(IOException::class)
fun <T, R> callRootable(
    path: Path,
    isAttributeAccess: Boolean,
    localObject: T,
    rootObject: T, block: T.() -> R
): R {
    path as? RootablePath ?: throw IllegalArgumentException("$path is not a RootablePath")
    return when (rootStrategy) {
        RootStrategy.NEVER -> localObject.block()
        RootStrategy.AUTOMATIC ->
            if (path.isRootRequired(isAttributeAccess)) {
                rootObject.block()
            } else {
                localObject.block()
            }
        RootStrategy.ALWAYS -> rootObject.block()
    }
}

@Throws(IOException::class)
fun <T, R> callRootable(
    path1: Path,
    path2: Path,
    isAttributeAccess: Boolean,
    localObject: T,
    rootObject: T,
    block: T.() -> R
): R {
    path1 as? RootablePath ?: throw IllegalArgumentException("$path1 is not a RootablePath")
    path2 as? RootablePath ?: throw IllegalArgumentException("$path2 is not a RootablePath")
    return when (rootStrategy) {
        RootStrategy.NEVER ->
            localObject.block()
        RootStrategy.AUTOMATIC ->
            if (path1.isRootRequired(isAttributeAccess)
                || path2.isRootRequired(isAttributeAccess)) {
                rootObject.block()
            } else {
                localObject.block()
            }
        RootStrategy.ALWAYS ->
            rootObject.block()
    }
}
