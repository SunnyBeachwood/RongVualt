/*
 * Copyright (c) 2026
 */

package me.zhanghai.android.files.viewer.markdown

import android.content.Context
import android.graphics.BitmapFactory
import android.graphics.drawable.BitmapDrawable
import android.net.Uri
import android.view.View
import io.noties.markwon.AbstractMarkwonPlugin
import io.noties.markwon.LinkResolver
import io.noties.markwon.Markwon
import io.noties.markwon.MarkwonConfiguration
import io.noties.markwon.ext.strikethrough.StrikethroughPlugin
import io.noties.markwon.ext.tables.TablePlugin
import io.noties.markwon.ext.tasklist.TaskListPlugin
import io.noties.markwon.image.ImageItem
import io.noties.markwon.image.ImagesPlugin
import io.noties.markwon.image.SchemeHandler
import io.noties.markwon.image.destination.ImageDestinationProcessor
import java.io.FileInputStream
import java8.nio.file.Path
import me.zhanghai.android.files.provider.common.newInputStream
import me.zhanghai.android.files.provider.linux.isLinuxPath

internal fun createMarkdownRenderer(
    context: Context,
    markdownFile: Path,
    onLinkClicked: (String) -> Unit
): Markwon {
    val imagePlugin = ImagesPlugin.create { plugin ->
        plugin.addSchemeHandler(LocalMarkdownImageSchemeHandler(context, markdownFile))
    }
    return Markwon.builder(context)
        .usePlugin(TablePlugin.create(context))
        .usePlugin(StrikethroughPlugin.create())
        .usePlugin(TaskListPlugin.create(context))
        .usePlugin(imagePlugin)
        .usePlugin(object : AbstractMarkwonPlugin() {
            override fun configureConfiguration(builder: MarkwonConfiguration.Builder) {
                builder.imageDestinationProcessor(object : ImageDestinationProcessor() {
                    override fun process(destination: String): String =
                        if (resolveMarkdownRelativePath(markdownFile, destination) != null) {
                            "$LOCAL_IMAGE_SCHEME:${Uri.encode(destination)}"
                        } else {
                            destination
                        }
                })
                builder.linkResolver(object : LinkResolver {
                    override fun resolve(view: View, link: String) {
                        onLinkClicked(link)
                    }
                })
            }
        })
        .build()
}

private class LocalMarkdownImageSchemeHandler(
    private val context: Context,
    private val markdownFile: Path
) : SchemeHandler() {
    override fun handle(raw: String, uri: Uri): ImageItem {
        val destination = Uri.decode(raw.substringAfter(':', ""))
        val imagePath = resolveMarkdownRelativePath(markdownFile, destination)
            ?: throw IllegalArgumentException("Not a local Markdown image: $raw")
        val resources = context.resources
        val maxWidth = resources.displayMetrics.widthPixels.coerceAtLeast(1)
        val maxHeight = resources.displayMetrics.heightPixels.coerceAtLeast(1)
        val bounds = BitmapFactory.Options().apply { inJustDecodeBounds = true }
        imagePath.openImageInputStream().use { BitmapFactory.decodeStream(it, null, bounds) }
        if (bounds.outWidth <= 0 || bounds.outHeight <= 0) {
            throw IllegalArgumentException("Unsupported image: $imagePath")
        }
        val options = BitmapFactory.Options().apply {
            inSampleSize = calculateInSampleSize(bounds.outWidth, bounds.outHeight, maxWidth, maxHeight)
        }
        val bitmap = imagePath.openImageInputStream().use {
            BitmapFactory.decodeStream(it, null, options)
        } ?: throw IllegalArgumentException("Cannot decode image: $imagePath")
        return ImageItem.withResult(BitmapDrawable(resources, bitmap))
    }

    override fun supportedSchemes(): Collection<String> = setOf(LOCAL_IMAGE_SCHEME)

    private fun Path.openImageInputStream() =
        if (isLinuxPath) FileInputStream(toString()) else newInputStream()
}

private fun calculateInSampleSize(width: Int, height: Int, maxWidth: Int, maxHeight: Int): Int {
    var sampleSize = 1
    while (width / (sampleSize * 2) >= maxWidth || height / (sampleSize * 2) >= maxHeight) {
        sampleSize *= 2
    }
    return sampleSize
}

private const val LOCAL_IMAGE_SCHEME = "markdown-local"
