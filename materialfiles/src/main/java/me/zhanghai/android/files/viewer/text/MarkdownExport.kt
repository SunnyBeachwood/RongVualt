package me.zhanghai.android.files.viewer.text

import android.content.Context
import android.graphics.Bitmap
import android.graphics.Canvas
import android.graphics.Color
import android.graphics.Paint
import android.graphics.Typeface
import android.graphics.pdf.PdfDocument
import android.text.Layout
import android.text.StaticLayout
import android.text.TextPaint
import java.io.File
import java.io.FileOutputStream
import java8.nio.file.Paths
import me.zhanghai.android.files.file.fileProviderUri
import org.commonmark.parser.Parser
import org.commonmark.renderer.html.HtmlRenderer

internal object MarkdownExport {
    private const val PAGE_WIDTH = 1240
    private const val PAGE_HEIGHT = 1754
    private const val MARGIN = 72
    private const val IMAGE_CHUNK_HEIGHT = 4096

    fun toHtml(markdown: String): String {
        val body = HtmlRenderer.builder().build().render(Parser.builder().build().parse(markdown))
        return "<!doctype html><html><head><meta charset=\"utf-8\"><meta name=\"viewport\" content=\"width=device-width\"></head><body>$body</body></html>"
    }

    fun writeHtml(context: Context, name: String, markdown: String): android.net.Uri {
        val file = exportFile(context, name, "html")
        file.writeText(toHtml(markdown), Charsets.UTF_8)
        return Paths.get(file.absolutePath).fileProviderUri
    }

    fun writePdf(context: Context, name: String, text: String): android.net.Uri {
        val paint = TextPaint(Paint.ANTI_ALIAS_FLAG).apply {
            color = Color.BLACK
            textSize = 28f
            typeface = Typeface.create(Typeface.MONOSPACE, Typeface.NORMAL)
        }
        val layout = staticLayout(text, paint, PAGE_WIDTH - MARGIN * 2)
        val document = PdfDocument()
        val contentHeight = PAGE_HEIGHT - MARGIN * 2
        val pages = maxOf(1, (layout.height + contentHeight - 1) / contentHeight)
        repeat(pages) { index ->
            val page = document.startPage(PdfDocument.PageInfo.Builder(PAGE_WIDTH, PAGE_HEIGHT, index + 1).create())
            page.canvas.drawColor(Color.WHITE)
            page.canvas.save()
            page.canvas.translate(MARGIN.toFloat(), (MARGIN - index * contentHeight).toFloat())
            page.canvas.clipRect(0, index * contentHeight, layout.width, (index + 1) * contentHeight)
            layout.draw(page.canvas)
            page.canvas.restore()
            document.finishPage(page)
        }
        val file = exportFile(context, name, "pdf")
        FileOutputStream(file).use(document::writeTo)
        document.close()
        return Paths.get(file.absolutePath).fileProviderUri
    }

    fun writeImages(
        context: Context,
        name: String,
        text: String,
        width: Int,
        viewportHeight: Int? = null,
    ): List<android.net.Uri> {
        val safeWidth = width.coerceAtLeast(480).coerceAtMost(2048)
        val paint = TextPaint(Paint.ANTI_ALIAS_FLAG).apply {
            color = Color.BLACK
            textSize = 32f
            typeface = Typeface.create(Typeface.MONOSPACE, Typeface.NORMAL)
        }
        val padding = 32
        val layout = staticLayout(text, paint, safeWidth - padding * 2)
        val fullHeight = (layout.height + padding * 2).coerceAtLeast(1)
        val outputHeight = viewportHeight?.coerceAtLeast(1)?.coerceAtMost(fullHeight) ?: fullHeight
        val chunkCount = (outputHeight + IMAGE_CHUNK_HEIGHT - 1) / IMAGE_CHUNK_HEIGHT
        return (0 until chunkCount).map { index ->
            val top = index * IMAGE_CHUNK_HEIGHT
            val height = minOf(IMAGE_CHUNK_HEIGHT, outputHeight - top)
            val bitmap = Bitmap.createBitmap(safeWidth, height, Bitmap.Config.ARGB_8888)
            val canvas = Canvas(bitmap)
            canvas.drawColor(Color.WHITE)
            canvas.save()
            canvas.translate(padding.toFloat(), (padding - top).toFloat())
            layout.draw(canvas)
            canvas.restore()
            val file = exportFile(context, "${name}_${index + 1}", "png")
            FileOutputStream(file).use { bitmap.compress(Bitmap.CompressFormat.PNG, 100, it) }
            bitmap.recycle()
            Paths.get(file.absolutePath).fileProviderUri
        }
    }

    private fun exportFile(context: Context, name: String, extension: String): File {
        val directory = File(context.cacheDir, "text-editor-share").apply { mkdirs() }
        val safeName = name.substringBeforeLast('.').replace(Regex("[^A-Za-z0-9._-]"), "_").ifEmpty { "document" }
        return File(directory, "${safeName}_${System.currentTimeMillis()}.$extension")
    }

    @Suppress("DEPRECATION")
    private fun staticLayout(text: String, paint: TextPaint, width: Int): StaticLayout =
        StaticLayout(text, paint, width, Layout.Alignment.ALIGN_NORMAL, 1.15f, 0f, false)
}
