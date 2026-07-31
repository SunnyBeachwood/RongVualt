package org.eds.veracrypt.ui

import android.view.View
import android.view.View.MeasureSpec
import android.view.ContextThemeWrapper
import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.platform.app.InstrumentationRegistry
import com.sovworks.eds.android.R
import com.sovworks.eds.android.databinding.ItemContainerCardBinding
import org.junit.Assert.assertTrue
import org.junit.Test
import org.junit.runner.RunWith

@RunWith(AndroidJUnit4::class)
class ContainerCardLayoutInstrumentationTest {
    @Test fun allActionsRemainInsideACardAt320dp() {
        val baseContext = InstrumentationRegistry.getInstrumentation().targetContext
        val context = ContextThemeWrapper(baseContext, R.style.Theme_VeraCrypt)
        val binding = ItemContainerCardBinding.inflate(android.view.LayoutInflater.from(context))
        binding.containerName.text = "A very long encrypted container name.img"
        binding.containerSummary.text = "Opened read-only for safe access."
        binding.primaryAction.setText(R.string.vc_browse_volume)
        binding.detailsAction.visibility = View.VISIBLE
        binding.lockAction.visibility = View.VISIBLE
        val width = (320 * context.resources.displayMetrics.density).toInt()

        binding.root.measure(MeasureSpec.makeMeasureSpec(width, MeasureSpec.EXACTLY), MeasureSpec.makeMeasureSpec(0, MeasureSpec.UNSPECIFIED))
        binding.root.layout(0, 0, width, binding.root.measuredHeight)

        listOf(binding.primaryAction, binding.detailsAction, binding.lockAction, binding.moreActions).forEach { action ->
            assertTrue("${action.contentDescription ?: action.javaClass.simpleName} starts outside the card", action.left >= 0)
            assertTrue("${action.contentDescription ?: action.javaClass.simpleName} exceeds the card", action.right <= width)
            assertTrue("Action must remain touchable", action.measuredHeight >= (48 * context.resources.displayMetrics.density).toInt())
        }
    }
}
